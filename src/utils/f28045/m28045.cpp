#include "f28045/m28045.h"
QVector<double> m28045::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
