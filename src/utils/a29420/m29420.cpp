#include "a29420/m29420.h"
QVector<double> m29420::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
