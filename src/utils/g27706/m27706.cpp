#include "g27706/m27706.h"
QVector<double> m27706::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
