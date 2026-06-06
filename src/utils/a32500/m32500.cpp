#include "a32500/m32500.h"
QVector<double> m32500::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
