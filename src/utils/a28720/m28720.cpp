#include "a28720/m28720.h"
QVector<double> m28720::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
