#include "a30260/m30260.h"
QVector<double> m30260::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
