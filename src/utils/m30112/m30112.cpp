#include "m30112/m30112.h"
QVector<double> m30112::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
