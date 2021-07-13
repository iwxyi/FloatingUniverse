#include "aninumberlabel.h"

AniNumberLabel::AniNumberLabel(QWidget *parent) : QLabel(parent)
{
    show_num = 0;
    setText(QString::number(show_num));
}

AniNumberLabel::AniNumberLabel(int num, QWidget *parent) : AniNumberLabel(parent)
{
    show_num = num;
    setText(QString::number(num));
}

void AniNumberLabel::setShowNum(int x)
{
    dn = x - show_num;
    show_num = x;
    this->setText(QString("%1").arg(x));

    QPropertyAnimation* animation = new QPropertyAnimation(this, "animation_progress");
    animation->setDuration(500);
    animation->setStartValue(0);
    animation->setEndValue(100);
    animation->start();
}

void AniNumberLabel::setAnimationProgress(int x)
{
    animation_progress = x;

    int n = show_num - dn * (100-animation_progress) / 100;
    this->setText(QString("%1").arg(n));
}

int AniNumberLabel::getAnimationProgress()
{
    return animation_progress;
}

void AniNumberLabel::resetAnimationProgress()
{
    animation_progress = 0;
}


