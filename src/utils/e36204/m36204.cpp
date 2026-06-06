#include "e36204/m36204.h"
QVector<double> m36204::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
