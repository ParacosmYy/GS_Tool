#include "a28560/m28560.h"
QVector<double> m28560::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
