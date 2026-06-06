#include "h9907/m9907.h"
QVector<double> m9907::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
