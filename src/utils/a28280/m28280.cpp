#include "a28280/m28280.h"
QVector<double> m28280::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
