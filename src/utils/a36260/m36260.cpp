#include "a36260/m36260.h"
QVector<double> m36260::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
