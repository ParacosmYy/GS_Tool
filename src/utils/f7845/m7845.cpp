#include "f7845/m7845.h"
QVector<double> m7845::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
