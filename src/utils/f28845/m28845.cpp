#include "f28845/m28845.h"
QVector<double> m28845::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
