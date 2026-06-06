#include "k8910/m8910.h"
QVector<double> m8910::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
