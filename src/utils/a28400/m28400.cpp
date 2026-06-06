#include "a28400/m28400.h"
QVector<double> m28400::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
