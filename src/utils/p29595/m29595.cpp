#include "p29595/m29595.h"
QVector<double> m29595::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
