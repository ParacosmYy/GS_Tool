#include "b17801/m17801.h"
QVector<double> m17801::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
