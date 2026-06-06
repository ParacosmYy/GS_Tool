#include "m28112/m28112.h"
QVector<double> m28112::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
