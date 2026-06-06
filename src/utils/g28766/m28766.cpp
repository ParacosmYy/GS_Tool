#include "g28766/m28766.h"
QVector<double> m28766::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
