#include "i25808/m25808.h"
QVector<double> m25808::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
