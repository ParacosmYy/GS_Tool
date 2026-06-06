#include "a29320/m29320.h"
QVector<double> m29320::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
