#include "a28240/m28240.h"
QVector<double> m28240::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
