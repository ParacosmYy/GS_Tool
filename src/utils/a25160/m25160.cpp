#include "a25160/m25160.h"
QVector<double> m25160::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
