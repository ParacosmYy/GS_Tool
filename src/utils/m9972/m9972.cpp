#include "m9972/m9972.h"
QVector<double> m9972::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
