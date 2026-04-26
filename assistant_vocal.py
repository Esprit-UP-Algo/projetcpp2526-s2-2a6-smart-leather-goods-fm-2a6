"""
Assistant vocal robuste (FR) avec :
- mode fichier (Qt) : faster-whisper en priorité, puis openai-whisper (léger, sans recharger tout le stack micro)
- mode boucle CLI : openai-whisper + fallback Google SpeechRecognition
- fuzzy matching des commandes via rapidfuzz

Compatible Python 3.10+
"""

from __future__ import annotations

import logging
import os
import tempfile
import argparse
from dataclasses import dataclass, field
from typing import Optional

import speech_recognition as sr
from rapidfuzz import process, fuzz

# Vocabulaire métier + phrases types (Whisper s’en sert comme contexte, pas comme sortie forcée).
PROMPT_METIER = (
    "Fil d'or, maroquinerie, ressources humaines, employés, clients, produits, "
    "stock, dépôt, planification, commande, atelier, couture, assemblage, finition, classification."
)
PROMPT_LONG = (
    "Transcription en français, assistant RH et gestion FIL D'OR. "
    "Phrases fréquentes exactes : "
    "donner la liste des employés, donner la liste des produits, afficher les employés, "
    "liste des clients, liste des produits, état du stock, stock du dépôt, "
    "planification de production, liste des commandes, ouvrir classification, ouvrir ancienneté. "
    + PROMPT_METIER
)

# Commandes canoniques (même sens que l’assistant Qt) — utilisées pour corriger une transcription proche.
COMMANDES_QT: list[str] = [
    "donner la liste des employes",
    "donner la liste des produits",
    "afficher les employes",
    "liste des clients",
    "liste des produits",
    "etat du stock",
    "stock du depot",
    "planification de production",
    "liste des commandes",
    "ouvrir classification",
    "ouvrir anciennete",
]


def _scorer_commande(query: str, choix: str, **kwargs: object) -> float:
    """
    WRatio + token_set_ratio (sans partial_ratio : trop de faux positifs sur texte halluciné).
    """
    return float(
        max(
            fuzz.WRatio(query, choix, **kwargs),
            fuzz.token_set_ratio(query, choix, **kwargs),
        )
    )


def aligner_commande_metier(texte: str, seuil: float = 68.0) -> str:
    """
    Si la transcription ressemble fortement à une commande métier, renvoie la forme canonique.
    Ne remplace que si le score dépasse le seuil (évite les faux positifs grossiers).
    """
    t = nettoyer_texte_brut(texte)
    if not t or not COMMANDES_QT:
        return t
    match = process.extractOne(t, COMMANDES_QT, scorer=_scorer_commande)
    if not match:
        return t
    commande, score, _ = match
    if score >= seuil:
        return commande
    return t


def nettoyer_texte_brut(texte: str) -> str:
    """Nettoie les transcriptions parasites / signatures connues."""
    if not texte:
        return ""
    t = " ".join(texte.split()).strip()
    tl = t.lower()
    parasites = (
        "sous-titres réalisés par la communauté d'amara.org",
        "sous titres realises par la communaute d'amara.org",
        "amara.org",
    )
    for p in parasites:
        if p in tl:
            return ""
    return t


def transcribe_wav_fichier(
    chemin_audio: str,
    model_size: str = "base",
    compute_type: str = "int8",
) -> Optional[str]:
    """
    Chemin rapide pour Qt : une seule transcription, sans initialiser PyAudio ni AssistantVocal.
    Ordre : faster-whisper (recommandé) -> openai-whisper.
    """
    if not chemin_audio or not os.path.exists(chemin_audio):
        return None

    log = logging.getLogger("transcribe_file")
    texte = ""

    try:
        from faster_whisper import WhisperModel

        log.info("STT fichier: faster-whisper model=%s", model_size)
        model = WhisperModel(model_size, device="cpu", compute_type=compute_type)
        # vad_filter=False : sur dictées courtes, le VAD coupe parfois la voix → hallucinations (« économies », etc.).
        segments, _info = model.transcribe(
            chemin_audio,
            language="fr",
            task="transcribe",
            beam_size=5,
            best_of=2,
            vad_filter=False,
            condition_on_previous_text=False,
            without_timestamps=True,
            no_speech_threshold=0.72,
            compression_ratio_threshold=2.4,
            initial_prompt=PROMPT_LONG,
        )
        texte = " ".join(s.text.strip() for s in segments if s.text and s.text.strip()).strip()
        texte = nettoyer_texte_brut(texte)
        texte = aligner_commande_metier(texte)
        if texte:
            return texte
        log.warning("faster-whisper: transcription vide, essai openai-whisper.")
    except Exception as e:  # noqa: BLE001
        log.warning("faster-whisper indisponible ou echec: %s: %s", type(e).__name__, e)

    try:
        import whisper

        log.info("STT fichier: openai-whisper model=%s", model_size)
        wmodel = whisper.load_model(model_size)
        result = wmodel.transcribe(
            chemin_audio,
            language="fr",
            fp16=False,
            initial_prompt=PROMPT_LONG,
            condition_on_previous_text=False,
        )
        texte = nettoyer_texte_brut((result.get("text") or "").strip())
        texte = aligner_commande_metier(texte)
        if texte:
            return texte
    except Exception as e:  # noqa: BLE001
        log.error("openai-whisper echec: %s: %s", type(e).__name__, e)

    return None


@dataclass
class AssistantVocal:
    """
    Assistant vocal en boucle :
    1) calibrer le micro
    2) écouter
    3) transcrire
    4) interpréter via fuzzy matching
    """

    micro_index: Optional[int] = None
    langue_whisper: str = "fr"
    langue_google: str = "fr-FR"
    modele_whisper: str = "small"  # "base" si machine plus lente
    timeout_ecoute: int = 10
    phrase_time_limit: int = 15
    seuil_fuzzy: int = 70
    recalibration_interval: int = 4
    liste_commandes: list[str] = field(
        default_factory=lambda: [
            "donner la liste des employes",
            "donner la liste des produits",
            "afficher les employes",
            "liste des clients",
            "liste des produits",
            "etat du stock",
            "stock du depot",
            "planification de production",
            "liste des commandes",
            "ouvrir classification",
            "ouvrir anciennete",
        ]
    )

    def __post_init__(self) -> None:
        # Logger simple lisible terminal.
        logging.basicConfig(
            level=logging.INFO,
            format="[%(levelname)s] %(message)s",
        )
        self.log = logging.getLogger("AssistantVocal")

        self.recognizer = sr.Recognizer()
        import whisper

        self.whisper_model = whisper.load_model(self.modele_whisper)
        self.log.info("Modele Whisper charge: %s", self.modele_whisper)
        self._ecoute_count = 0

        self.lister_micros()

        if self.micro_index is None:
            self.micro_index = 0
            self.log.info("Aucun index micro fourni, utilisation index=0")

    def lister_micros(self) -> None:
        """Affiche les micros disponibles pour sélectionner le bon index."""
        micros = sr.Microphone.list_microphone_names()
        if not micros:
            self.log.warning("Aucun micro detecte par SpeechRecognition.")
            return
        self.log.info("Micros disponibles :")
        for i, nom in enumerate(micros):
            self.log.info("  [%d] %s", i, nom)

    def calibrer(self) -> None:
        """Calibration du bruit ambiant avant écoute."""
        try:
            with sr.Microphone(device_index=self.micro_index) as source:
                self.log.info("Calibration bruit ambiant (1s)...")
                self.recognizer.adjust_for_ambient_noise(source, duration=1)
                self.log.info(
                    "Calibration OK. energy_threshold=%.2f",
                    self.recognizer.energy_threshold,
                )
        except Exception as e:  # noqa: BLE001
            self.log.error("Erreur calibration: %s: %s", type(e).__name__, e)

    def ecouter(self) -> Optional[sr.AudioData]:
        """
        Capture l'audio micro avec timeout élargi.
        Retourne AudioData ou None si erreur/timeout.
        """
        try:
            with sr.Microphone(device_index=self.micro_index) as source:
                # Recalibrage périodique (pas à chaque tour, sinon instable/lent).
                if self._ecoute_count % max(1, self.recalibration_interval) == 0:
                    self.recognizer.adjust_for_ambient_noise(source, duration=0.6)
                    self.log.info(
                        "Recalibration courte. energy_threshold=%.2f",
                        self.recognizer.energy_threshold,
                    )
                self.log.info("Ecoute... (timeout=%ss, phrase_limit=%ss)", self.timeout_ecoute, self.phrase_time_limit)
                audio = self.recognizer.listen(
                    source,
                    timeout=self.timeout_ecoute,
                    phrase_time_limit=self.phrase_time_limit,
                )
                self._ecoute_count += 1
                self.log.info("Audio capte avec succes.")
                return audio
        except sr.WaitTimeoutError as e:
            self.log.warning("WaitTimeoutError: %s", e)
        except Exception as e:  # noqa: BLE001
            self.log.error("Erreur ecoute (%s): %s", type(e).__name__, e)
        return None

    def transcrire(self, audio: sr.AudioData) -> Optional[str]:
        """
        STT principal via Whisper local.
        Fallback Google si Whisper échoue.
        """
        if audio is None:
            return None

        temp_path = None
        try:
            # Convertit l'audio capté en WAV temporaire lisible par Whisper.
            with tempfile.NamedTemporaryFile(delete=False, suffix=".wav") as tmp:
                tmp.write(audio.get_wav_data())
                temp_path = tmp.name

            self.log.info("Transcription Whisper...")
            result = self.whisper_model.transcribe(temp_path, language=self.langue_whisper)
            texte = (result.get("text") or "").strip()
            if texte:
                self.log.info("Texte transcrit (Whisper): %s", texte)
                return texte

            self.log.warning("Whisper a retourne un texte vide, fallback Google.")
        except Exception as e:  # noqa: BLE001
            self.log.error("Erreur Whisper (%s): %s", type(e).__name__, e)
            self.log.info("Fallback Google STT...")
        finally:
            if temp_path and os.path.exists(temp_path):
                try:
                    os.remove(temp_path)
                except OSError:
                    pass

        # Fallback Google SpeechRecognition.
        try:
            texte_google = self.recognizer.recognize_google(audio, language=self.langue_google).strip()
            if texte_google:
                self.log.info("Texte transcrit (Google): %s", texte_google)
                return texte_google
        except sr.UnknownValueError as e:
            self.log.warning("UnknownValueError (Google): %s", e)
        except sr.RequestError as e:
            self.log.error("RequestError (Google): %s", e)
        except Exception as e:  # noqa: BLE001
            self.log.error("Erreur Google STT (%s): %s", type(e).__name__, e)
        return None

    def transcrire_fichier(self, chemin_audio: str) -> Optional[str]:
        """
        Transcription one-shot d'un fichier audio WAV/MP3/etc via Whisper.
        Utilisé par Qt (QProcess) pour transcrire un enregistrement temporaire.
        """
        if not chemin_audio or not os.path.exists(chemin_audio):
            self.log.error("Fichier audio introuvable: %s", chemin_audio)
            return None
        try:
            self.log.info("Transcription fichier Whisper: %s", chemin_audio)
            result = self.whisper_model.transcribe(chemin_audio, language=self.langue_whisper)
            texte = (result.get("text") or "").strip()
            texte = self.nettoyer_texte(texte)
            if texte:
                self.log.info("Texte transcrit (fichier): %s", texte)
                return texte
            self.log.warning("Whisper a retourne un texte vide (fichier).")
            return None
        except Exception as e:  # noqa: BLE001
            self.log.error("Erreur transcrire_fichier (%s): %s", type(e).__name__, e)
            return None

    @staticmethod
    def nettoyer_texte(texte: str) -> str:
        """Nettoie les transcriptions parasites / signatures connues."""
        return nettoyer_texte_brut(texte)

    @staticmethod
    def corriger_commande(texte_brut: str, liste_commandes: list[str], seuil: int = 70) -> Optional[str]:
        """
        Retourne la commande la plus proche si score >= seuil, sinon None.
        """
        if not texte_brut or not liste_commandes:
            return None
        match = process.extractOne(
            texte_brut,
            liste_commandes,
            scorer=fuzz.WRatio,
        )
        if not match:
            return None
        commande, score, _ = match
        if score >= seuil:
            return commande
        return None

    def interpreter(self, texte: str) -> Optional[str]:
        """Interprète le texte par fuzzy matching."""
        texte = self.nettoyer_texte(texte)
        if not texte:
            self.log.warning("Texte ignore (parasite ou vide).")
            return None
        commande = self.corriger_commande(texte, self.liste_commandes, self.seuil_fuzzy)
        if commande:
            self.log.info("Interprete: %s", commande)
        else:
            self.log.info("Interprete: aucune commande proche (texte libre).")
        return commande

    def run(self) -> None:
        """
        Boucle principale :
        - ne crash jamais
        - revient toujours à l'écoute après erreur
        """
        self.calibrer()
        self.log.info("Assistant vocal demarre. Ctrl+C pour quitter.")
        while True:
            try:
                audio = self.ecouter()
                if audio is None:
                    continue

                texte = self.transcrire(audio)
                if not texte:
                    self.log.warning("Aucun texte exploitable.")
                    continue

                _commande = self.interpreter(texte)
                # Ici tu peux brancher l'appel à ton assistant/chat Qt.
                # Exemple: envoyer 'texte' au backend conversationnel.
            except KeyboardInterrupt:
                self.log.info("Arret demande par l'utilisateur.")
                break
            except Exception as e:  # noqa: BLE001
                self.log.error("Erreur boucle run (%s): %s", type(e).__name__, e)
                # Toujours continuer la boucle.
                continue


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Assistant vocal FR")
    parser.add_argument("--transcribe-file", dest="transcribe_file", help="Transcrire un fichier audio puis quitter")
    parser.add_argument(
        "--model",
        default="base",
        help="Modele whisper (fichier + boucle): tiny (rapide), base (recommandé), small, ...",
    )
    parser.add_argument(
        "--compute-type",
        default="int8",
        help="Pour faster-whisper: int8, int8_float32, float32, ...",
    )
    parser.add_argument("--micro-index", type=int, default=None, help="Index du micro pour mode boucle")
    args = parser.parse_args()

    if args.transcribe_file:
        # Ne pas construire AssistantVocal : évite PyAudio + chargement inutile à chaque clic micro.
        logging.basicConfig(level=logging.INFO, format="[%(levelname)s] %(message)s")
        texte = transcribe_wav_fichier(
            args.transcribe_file,
            model_size=args.model,
            compute_type=args.compute_type,
        )
        if texte:
            # IMPORTANT pour Qt: sortie standard = texte final uniquement.
            print(texte, flush=True)
            raise SystemExit(0)
        raise SystemExit(3)

    assistant = AssistantVocal(
        micro_index=args.micro_index,
        modele_whisper=args.model,
        seuil_fuzzy=70,
    )
    assistant.run()

