#include "c7902/m7902.h"
QVector<double> m7902::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
