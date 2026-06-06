#include "m29832/m29832.h"
QVector<double> m29832::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
