#include "doctest.h"

#include <map>
#include <stdexcept>
#include <string>
#include "RequestSummaryBuilder.h"
#include "WorkflowRequest.h"
#include "WorkflowRequestBuilder.h"
#include "WorkflowRequestDirector.h"

namespace {
map<string, string> labInput() {
    map<string, string> in;
    in["name"] = "Blood panel";
    in["requesterId"] = "12";
    in["riskLevel"] = "2";
    in["equipment"] = "Centrifuge";
    return in;
}

map<string, string> loanInput() {
    map<string, string> in;
    in["name"] = "Personal loan";
    in["requesterId"] = "40";
    in["loanAmount"] = "50000";
    return in;
}
}  // namespace

// ------------------------------------------------------------------ product

TEST_CASE("Builder: a request returns empty text for an attribute it does not have") {
    WorkflowRequest request;
    CHECK(request.getAttribute("missing") == "");
    request.setAttribute("riskLevel", "3");
    CHECK(request.getAttribute("riskLevel") == "3");
}

TEST_CASE("Builder: the two-argument constructor sets name and requester") {
    WorkflowRequest request("Blood panel", 12);
    CHECK(request.getName() == "Blood panel");
    CHECK(request.getRequesterId() == 12);
    CHECK(request.getDeadlineDays() == 0);
}

// ------------------------------------------------------------------ concrete builders

TEST_CASE("Builder: the request builder assembles every part it is given") {
    WorkflowRequestBuilder builder;
    builder.reset();
    builder.buildHeader("Blood panel", 12);
    builder.buildAttribute("riskLevel", "2");
    builder.buildDeadline(7);
    builder.buildPriority(3);

    WorkflowRequest request = builder.getResult();

    CHECK(request.getName() == "Blood panel");
    CHECK(request.getRequesterId() == 12);
    CHECK(request.getAttribute("riskLevel") == "2");
    CHECK(request.getDeadlineDays() == 7);
    CHECK(request.getPriority() == 3);
}

// ------------------------------------------------------------------ director

TEST_CASE("Director: a standard request gets a 14-day deadline and priority 1") {
    WorkflowRequestBuilder builder;
    WorkflowRequestDirector director(&builder);

    director.constructStandard(labInput());
    WorkflowRequest request = builder.getResult();

    CHECK(request.getName() == "Blood panel");
    CHECK(request.getRequesterId() == 12);
    CHECK(request.getAttribute("riskLevel") == "2");
    CHECK(request.getAttribute("equipment") == "Centrifuge");
    CHECK(request.getDeadlineDays() == 14);
    CHECK(request.getPriority() == 1);
}

TEST_CASE("Director: an urgent request gets a 2-day deadline and priority 5") {
    WorkflowRequestBuilder builder;
    WorkflowRequestDirector director(&builder);

    director.constructUrgent(loanInput());
    WorkflowRequest request = builder.getResult();

    CHECK(request.getAttribute("loanAmount") == "50000");
    CHECK(request.getDeadlineDays() == 2);
    CHECK(request.getPriority() == 5);
}

TEST_CASE("Director: name and requesterId are not copied into the attributes") {
    WorkflowRequestBuilder builder;
    WorkflowRequestDirector director(&builder);
    director.constructStandard(labInput());
    WorkflowRequest request = builder.getResult();
    CHECK(request.getAttribute("name") == "");
    CHECK(request.getAttribute("requesterId") == "");
}

TEST_CASE("Director: the same process produces different representations") {
    WorkflowRequestBuilder requestBuilder;
    RequestSummaryBuilder summaryBuilder;
    WorkflowRequestDirector director(&requestBuilder);

    director.constructUrgent(labInput());
    director.setBuilder(&summaryBuilder);
    director.constructUrgent(labInput());

    WorkflowRequest request = requestBuilder.getResult();
    string summary = summaryBuilder.getResult();

    CHECK(request.getName() == "Blood panel");
    CHECK(summary.find("Request: Blood panel (requester 12)") != string::npos);
    CHECK(summary.find("riskLevel: 2") != string::npos);
    CHECK(summary.find("Deadline: 2 days") != string::npos);
    CHECK(summary.find("Priority: 5") != string::npos);
}

TEST_CASE("Director: reusing a builder does not leak data from the previous build") {
    WorkflowRequestBuilder requestBuilder;
    RequestSummaryBuilder summaryBuilder;
    WorkflowRequestDirector director(&requestBuilder);

    director.constructStandard(labInput());
    director.constructStandard(loanInput());
    WorkflowRequest request = requestBuilder.getResult();
    CHECK(request.getName() == "Personal loan");
    CHECK(request.getAttribute("riskLevel") == "");          // from the first build

    director.setBuilder(&summaryBuilder);
    director.constructStandard(labInput());
    director.constructStandard(loanInput());
    CHECK(summaryBuilder.getResult().find("Blood panel") == string::npos);
}

TEST_CASE("Director: missing name and requester fall back to defaults") {
    WorkflowRequestBuilder builder;
    WorkflowRequestDirector director(&builder);
    map<string, string> in;
    in["notes"] = "walk-in";

    director.constructStandard(in);
    WorkflowRequest request = builder.getResult();

    CHECK(request.getName() == "");
    CHECK(request.getRequesterId() == 0);
    CHECK(request.getAttribute("notes") == "walk-in");
}

TEST_CASE("Director: a requester id that is not a number is rejected") {
    WorkflowRequestBuilder builder;
    WorkflowRequestDirector director(&builder);
    map<string, string> in = labInput();
    in["requesterId"] = "twelve";
    CHECK_THROWS_AS(director.constructStandard(in), std::invalid_argument);
}

TEST_CASE("Director: constructing without a builder throws instead of crashing") {
    WorkflowRequestDirector director(nullptr);
    CHECK_THROWS_AS(director.constructStandard(labInput()), std::invalid_argument);
    CHECK_THROWS_AS(director.constructUrgent(labInput()), std::invalid_argument);
}
