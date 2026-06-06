#include "j18429/m18429.h"
QVector<double> m18429::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
