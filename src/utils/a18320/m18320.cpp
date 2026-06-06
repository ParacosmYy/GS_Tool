#include "a18320/m18320.h"
QVector<double> m18320::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
