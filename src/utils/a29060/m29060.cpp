#include "a29060/m29060.h"
QVector<double> m29060::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
