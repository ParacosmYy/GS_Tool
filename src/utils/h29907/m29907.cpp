#include "h29907/m29907.h"
QVector<double> m29907::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
