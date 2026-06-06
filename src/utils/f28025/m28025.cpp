#include "f28025/m28025.h"
QVector<double> m28025::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
