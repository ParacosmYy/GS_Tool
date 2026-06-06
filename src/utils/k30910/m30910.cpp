#include "k30910/m30910.h"
QVector<double> m30910::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
