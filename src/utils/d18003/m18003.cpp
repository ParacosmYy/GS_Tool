#include "d18003/m18003.h"
QVector<double> m18003::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
