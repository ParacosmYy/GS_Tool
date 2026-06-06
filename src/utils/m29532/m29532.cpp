#include "m29532/m29532.h"
QVector<double> m29532::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
