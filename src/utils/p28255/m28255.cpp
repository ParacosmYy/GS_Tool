#include "p28255/m28255.h"
QVector<double> m28255::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
