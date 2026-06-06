#include "h21607/m21607.h"
QVector<double> m21607::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
