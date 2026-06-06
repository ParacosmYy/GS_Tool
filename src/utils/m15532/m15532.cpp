#include "m15532/m15532.h"
QVector<double> m15532::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
