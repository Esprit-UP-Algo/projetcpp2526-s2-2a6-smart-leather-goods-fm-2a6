from sqlalchemy import create_engine, Column, Integer, String, Float, Text
from sqlalchemy.orm import declarative_base, sessionmaker
from app.config import settings

engine = create_engine(settings.db_url, future=True)
SessionLocal = sessionmaker(bind=engine, autoflush=False, autocommit=False)
Base = declarative_base()


class PredictionCache(Base):
    __tablename__ = "prediction_cache"

    id = Column(Integer, primary_key=True)
    year = Column(Integer, index=True, unique=True)
    payload = Column(Text)
    confidence = Column(Float)
    top_style = Column(String(128))


def init_db() -> None:
    Base.metadata.create_all(bind=engine)
