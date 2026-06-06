#include "m29272/m29272.h"
QVector<double> m29272::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
