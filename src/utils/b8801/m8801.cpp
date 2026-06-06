#include "b8801/m8801.h"
QVector<double> m8801::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
