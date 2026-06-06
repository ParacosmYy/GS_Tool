#include "g35066/m35066.h"
QVector<double> m35066::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
