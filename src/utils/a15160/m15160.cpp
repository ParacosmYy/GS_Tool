#include "a15160/m15160.h"
QVector<double> m15160::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
