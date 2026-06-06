#include "m29752/m29752.h"
QVector<double> m29752::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
