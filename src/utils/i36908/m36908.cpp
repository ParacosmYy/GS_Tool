#include "i36908/m36908.h"
QVector<double> m36908::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
