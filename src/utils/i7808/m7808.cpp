#include "i7808/m7808.h"
QVector<double> m7808::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
