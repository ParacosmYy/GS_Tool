#include "m27112/m27112.h"
QVector<double> m27112::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
