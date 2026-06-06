#include "h19907/m19907.h"
QVector<double> m19907::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
