#include "i36628/m36628.h"
QVector<double> m36628::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
