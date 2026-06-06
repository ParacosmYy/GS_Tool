#include "f8845/m8845.h"
QVector<double> m8845::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
