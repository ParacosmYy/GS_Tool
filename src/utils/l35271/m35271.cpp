#include "l35271/m35271.h"
QVector<double> m35271::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
