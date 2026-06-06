#include "m29012/m29012.h"
QVector<double> m29012::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
