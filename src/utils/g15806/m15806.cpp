#include "g15806/m15806.h"
QVector<double> m15806::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
