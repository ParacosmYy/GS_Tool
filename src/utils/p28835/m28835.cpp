#include "p28835/m28835.h"
QVector<double> m28835::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
