#include "a18180/m18180.h"
QVector<double> m18180::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
