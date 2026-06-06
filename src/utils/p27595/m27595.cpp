#include "p27595/m27595.h"
QVector<double> m27595::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
