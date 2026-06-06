#include "m37752/m37752.h"
QVector<double> m37752::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
