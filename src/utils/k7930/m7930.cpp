#include "k7930/m7930.h"
QVector<double> m7930::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
