/*
// Copyright 2018 Oleksandr Grytsov
// Modifications copyright (c) 2020, Cascoda Ltd.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
*/

#include "port/oc_assert.h"

void exit_impl(int status)
{
	(void)status;
	while (1)
		;
}

void abort_impl()
{
	while (1)
		;
}