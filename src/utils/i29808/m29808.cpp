#include "i29808/m29808.h"
QVector<double> m29808::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
