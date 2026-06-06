#include "j35609/m35609.h"
QVector<double> m35609::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
