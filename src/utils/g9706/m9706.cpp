#include "g9706/m9706.h"
QVector<double> m9706::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
